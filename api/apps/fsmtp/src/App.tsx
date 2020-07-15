import React from 'react';
import InboxPage from './pages/Inbox.page';
import { Switch, Route } from 'react-router-dom';
import Meta from './services/Meta.service';

import Header from './components/Header.component';

export default class App extends React.Component
{
  public constructor(props: any)
  {
    super(props);
  }
  
  public render(): any
  {
    return (
      <div className="router">
        {/* The default overlay / navigation */}
        <Header />
        {/* The router which switches the pages */}
        <Switch>
          <Route exact path="/inbox" component={InboxPage} />
        </Switch>
      </div>
    );
  }
}
